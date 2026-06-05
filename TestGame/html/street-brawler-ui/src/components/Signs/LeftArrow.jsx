export const LeftArrow = ({ active }) => {
  if(active)
    return (
      <div className={`arrow left ${active ? 'active' : ''}`}>
        <img src="signals/left-arrow.png" alt="Left Arrow" />
      </div>
    );
  else return null;
}
